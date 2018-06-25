begin_array     = "\[".
begin_object    = "{".
end_array       = "\]".
end_object      = "}".
name_separator  = ":".
value_separator = ",".
atomic_value = "(false)|(true)|(null)|(-?(0|[1-9][0-9]*)(\.[0-9]+)?([Ee][+-]?[0-9]+)?)".
string = "\"([^\"\\]|\\.)*\"".
IGNORE "[ \t\n\r]+".

value: atomic_value; string; object; array.

object: begin_object, (string, name_separator, value) CHAIN value_separator, end_object.

array: begin_array, value CHAIN value_separator, end_array.
