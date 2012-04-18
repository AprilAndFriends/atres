Formatting is applied using BBCode.
Commands are specified in [] brackets which are together called tags.
Sequences contain one opening command [x] and the appropriate closing command [/x].
Nesting commands are possible.
Interleaving commands (like [x][y][/x][/y]) are not possible (the [/x] will be disregarded).
The command and the parameter can be separated with any character.

Following formatting commands are supported:

n - normal (no shadow / border)
s - shadow
b - border
c - color change; parameter: string with hex value for color in RGB or RGBA
f - font change; parameter: string of the font name definition including optional scale factor

Examples:

The word [b]border[/b] has a border.
This [f arial:0.9]word[/f] is displayed in "arial" with a scaling factor 0.9.
The color of the word at the end is [c:00FF00]green[/c].


Notes:

- Font changes will not change line height. Keep that in mind when using fonts of different sizes.
- After closing a formatting sequence, the previous formatting sequence will be continued (stacking formatting sequences).
- Improperly formed formatting code will be regarded as normal text.
- Sequences that remain open will be automatically closed at the end.
- The character ] will be printed normally if it is not used as closing for a formatting command.
- The character [ can be printed using the empty formatting command [].
- Parameters follow the command type after any single character (preferably space " ", colon ":" or equal "=" for the sake of readability).
