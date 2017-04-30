#![allow(non_upper_case_globals)]
#![allow(dead_code)]
#![allow(non_snake_case)]

use std::io;
use std::io::Read;
use std::io::BufReader;
use std::collections::HashMap;
use std::str;

%initcode

macro_rules! llerror {
	($parser:expr, $($z:expr),+) => {
		print!("error: {}:{}:{}: ", $parser.file_name, $parser.position.line_number, $parser.position.column);
		$(
			print!("{}", $z);
		)*
		println!();
	}
}
%%const endOfInput: u32 = %N;
const nrTokens: u32 = endOfInput + 1;
%%const llTokenSetSize: usize = %t;

type LLTokenSet = [u32; llTokenSetSize];

fn uniteTokenSets(b: LLTokenSet, c: LLTokenSet) -> LLTokenSet {
	let mut a: LLTokenSet = [0; llTokenSetSize];

	for i in 0..llTokenSetSize {
		a[i] = b[i] | c[i];
	}
	a
}

fn tokenInCommon(a: LLTokenSet, b: LLTokenSet) -> bool {
	for i in 0..llTokenSetSize {
		if a[i] & b[i] != 0 {
			return true;
		}
	}
	false
}

fn memberTokenSet(token: u32, set: LLTokenSet) -> bool {
	set[(token / 32) as usize] & (1 << (token % 32)) != 0
}

%%const endOfInputSet: [u32; %t] = [%E]; 

type KeywordList = HashMap<u32, HashMap<String, u32>>;

fn build_keyword_list() -> KeywordList {
	let mut keyword_list = KeywordList::new();

%keywordlist
	keyword_list
}
%tokensets

struct DFAState {
	ch: u8,
	next: usize,
	llTokenSet: Option<LLTokenSet>
}

fn get_scan_tab() -> Box<Vec<DFAState>> {
	return Box::new(vec![
%scantable
	])
}

%createtokensets

fn not_empty(tSet: LLTokenSet) -> bool {
	if tSet[0] > 1 {
		return true
	}
	for i in 1 .. llTokenSetSize {
		if tSet[i] != 0 {
			return true;
		}
	}
	false
}

#[derive(Clone, Copy, Default)]
struct SymbolPos {
	line_number: u32,
	column: u32
}

struct Parser<'self_lifetime, R: io::Read> {
	reader: BufReader<R>,
    file_name: &'self_lifetime str,
	position: SymbolPos,
	errorOccurred: bool,
	llNrCharactersRead: u32,
	scan_buffer: Vec<u8>,
	currSymbol: LLTokenSet,
	lastSymbol: String,
	lastSymbolPos: SymbolPos,
	buffer_end: usize,
	buffer_fill: usize,
	at_eof: bool,
	keyword_list: KeywordList,
	scan_tab: Box<Vec<DFAState>>
}

impl<'self_lifetime, T: io::Read> Parser<'self_lifetime, T> {

%code1
    
	fn get_next_character(&mut self, ch: &mut u8) -> bool {
		let mut buf: [u8; 1] = [0; 1];

		match self.reader.read(&mut buf) {
			Ok(n) if n == 1 => {
				*ch = buf[0];
				return true;
			}
			_ => {
				return false;
			}
		}
	}

	fn next_state(&self, state: &mut usize, ch: u8) -> Option<LLTokenSet> {
		let mut i = *state;

		while self.scan_tab[i].ch != ch && self.scan_tab[i].ch != 0xff {
			i += 1;
		}
		*state = self.scan_tab[i].next;
		self.scan_tab[i].llTokenSet
	}

%{keywords
	fn ll_keyword(&self, tokenSet: LLTokenSet) -> LLTokenSet {
		// Parse current scan buffer as string, and look up in keyword list for
		// the recognized tokens
		str::from_utf8(&self.scan_buffer[0 .. self.buffer_end]).ok().map(
			|keyword_text| {
			for i in 0 .. nrTokens - 1 {
				if memberTokenSet(i, tokenSet) {
					match self.keyword_list.get(&i) {
						Some(keyword_map) => {
							match keyword_map.get(keyword_text) {
								Some(keyword_index) => {
									let mut llKeyWordSet = [0; llTokenSetSize];
									llKeyWordSet[(keyword_index / 32) as usize] = 1 << (keyword_index % 32);
									return llKeyWordSet
								},
								None => ()
							}
						},
						None => ()
					}
				}
			}
			tokenSet
		}).unwrap_or(tokenSet)
	}

%}keywords
	fn check_line_feeds(&mut self) {
		let mut lastNlPos: usize = 0;

		for i in 0 .. self.buffer_end {
			if self.scan_buffer[i] == 10 { // '\n' is ASCII 10; not unicode safe?
				lastNlPos = i;
				self.position.line_number += 1;
				self.position.column = 0;
			}
		}
		self.position.column += (self.buffer_end - lastNlPos) as u32;
	}

	// Implementation of buffer split doesn't seem efficient, but using e.g.
	// split_at() complains about self.scan_tab already being borrowed, and
	// split_off() does the wrong thing.
	fn next_symbol(&mut self) {

		// Copy last recognized symbol into buffer and adjust positions
		let mut bytes = Vec::<u8>::with_capacity(self.buffer_end);
		for byte in self.scan_buffer.iter().take(self.buffer_end) {
			bytes.push(*byte);
		}
		match String::from_utf8(bytes) {
			Ok(str) => self.lastSymbol = str,
			Err(_) => self.lastSymbol = "illegal character sequence".to_string()
		}
		self.lastSymbolPos = self.position;
		// Count nr line feeds in buffer
		self.check_line_feeds();
		// move remains of scan_buffer to beginning
		let mut bytes = Vec::<u8>::with_capacity(self.scan_buffer.len() - self.buffer_end);
		for byte in self.scan_buffer.iter().skip(self.buffer_end) {
			bytes.push(*byte);
		}
		self.scan_buffer = bytes;
		self.buffer_fill -= self.buffer_end;
		self.buffer_end = 0;

		let mut buffer_pos: usize = 0;
		let mut ch: u8 = 0;
		let mut state: usize = 0;
		let mut recognized_token: Option<LLTokenSet> = None;
		while buffer_pos != self.buffer_fill || !self.at_eof {
			if buffer_pos != self.buffer_fill {
				ch = self.scan_buffer[buffer_pos];
				buffer_pos += 1;
			} else if self.at_eof || !self.get_next_character(&mut ch) {
				self.at_eof = true;
			} else {
				self.scan_buffer.push(ch);
				buffer_pos += 1;
				self.buffer_fill += 1;
			}
			if self.at_eof {
				state = 0xffffffff;
			} else {
				if let Some(token) = self.next_state(&mut state, ch) {
					recognized_token = Some(token);
					self.buffer_end = buffer_pos;
				}
			}
			if state == 0xffffffff {
				if self.at_eof && self.buffer_fill == 0 {
					self.currSymbol = endOfInputSet;
					return;
				}
				match recognized_token {
					Some(token) => {
						if not_empty(token) {
%{keywords
							self.currSymbol = self.ll_keyword(token);
%}keywords
%{!keywords
							self.currSymbol = token;
%}!keywords
							return;
						}
					},
					None => {
						let ch: char = char::from(self.scan_buffer[0]);
						llerror!(self, "Illegal character: '", ch, "'");
						self.buffer_end = 1;
					}
				}
				/* If nothing recognized, continue; no need to copy buffer */
				self.check_line_feeds();
				self.buffer_fill -= self.buffer_end;
				let buffer_rest = self.scan_buffer.split_off(self.buffer_end);
				self.scan_buffer = buffer_rest;
				recognized_token = None;
				state = 0;
				self.buffer_end = 0;
				buffer_pos = 0;
			}
		}
		self.currSymbol = endOfInputSet;
	}

	fn waitForToken(&mut self, set: LLTokenSet, follow: LLTokenSet) {
		let ltSet: [u32; 1] = uniteTokenSets(set, follow);

		while self.currSymbol != endOfInputSet && !tokenInCommon(self.currSymbol, ltSet) {
			self.next_symbol();
			llerror!(self, "token skipped: ", self.lastSymbol);
		}
	}

	fn getToken(&mut self, token: u32, set: LLTokenSet, follow: LLTokenSet) {
		let ltSet = uniteTokenSets(set, follow);

		while self.currSymbol != endOfInputSet && !memberTokenSet(token, self.currSymbol) && !tokenInCommon(self.currSymbol, ltSet) {
			self.next_symbol();
			if !memberTokenSet(0, self.currSymbol) {
				llerror!(self, "token skipped: ", self.lastSymbol);
			}
		}
		if !memberTokenSet(token, self.currSymbol) {
			llerror!(self, "token expected: ", tokenName[token as usize]);
		} else {
			self.next_symbol();
		}
	}
%parsefunctionbodies
}

pub fn parse<T>(r: T, file_name: &str) where T: io::Read {
	let mut parser = Parser {
		reader: BufReader::new(r),
        file_name: file_name,
		position: SymbolPos {
			line_number: 1,
			column: 1
		},
		errorOccurred: false,
		llNrCharactersRead: 0,
		scan_buffer: Vec::<u8>::with_capacity(8192),
		currSymbol: [0; llTokenSetSize],
		lastSymbol: String::new(),
		lastSymbolPos: SymbolPos {
			line_number: 0,
			column: 0
		},
		buffer_end: 0,
		buffer_fill: 0,
		at_eof: false,
		keyword_list: build_keyword_list(),
		scan_tab: get_scan_tab()
	};

	parser.next_symbol();
%%    parser.%S(#S);
	if !memberTokenSet(endOfInput, parser.currSymbol)
	{
		llerror!(parser, "end of input expected");
	}
}
%{main

fn main() {
	parser::parse(io::stdin())
}
%}main
%exitcode
