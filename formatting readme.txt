Formatting is applied using BBCode.
Commands are specified in [] brackets which are together called tags.
Sequences contain one opening command [x] and the appropriate closing command [/x].
Nesting commands are possible.
Interleaving commands (like [x][y][/x][/y]) are not possible (the [/x] will be disregarded).
The command and the parameter can be separated with any character.

Following formatting commands are supported:

n	- normal (no shadow / border)
s	- shadow; parameter (optional): string with hex value for color in RGB or RGBA
              parameter (optional): shadow offset as "X,Y" multiplier (counted as 1 parameter)
b	- border; parameter (optional): string with hex value for color in RGB or RGBA
              parameter (optional): border thickness multiplier
t	- strike-through; parameter (optional): string with hex value for color in RGB or RGBA
                      parameter (optional): strike-through thickness multiplier
u	- underline; parameter (optional): string with hex value for color in RGB or RGBA
                 parameter (optional): underline thickness multiplier
l	- italic
h	- hide
c	- color change; parameter: string with hex value for color in RGB
f	- font change; parameter: string of the font name definition including optional scale factor
i	- icon; parameter: string of the icon font name definition including optional scale factor
            inside-tag: string of the the icon name (from the filename without extension)
-	- disable [] formatting until this command tag is closed

Examples:

The word [b]border[/b] has a border. The word [b=0000FF]blue[/b] has a blue border.
The word [b:FF0000,1.5]thicker red border[/b] has a 1.5 times thicker red border than the default.
The word [b:,1.5]thicker border[/b] has a 1.5 times thicker border than the default with default color.
This [f arial:0.9]word[/f] is displayed in "arial" with a scaling factor 0.9.
The color of the word at the end is [c:00FF00]green[/c].
Press [i:buttons]x_button[/i] to delete "C:\Windows\system32".
This [u:FFFF00,2]underline[/u] is yellow and 2 pixels thick.
This [t:,3]strike-through[/t] is 3 pixels thick, but uses the text color.
[l]Italic[/l] text also [l]works[/l].
This next piece of text has hi[h]d[/h]den char[h]act[/h]ers.


Notes:

- Font changes will not change line height. Keep that in mind when using fonts of different sizes.
- After closing a formatting sequence, the previous formatting sequence will be continued (stacking
  formatting sequences).
- Improperly formed formatting code will be regarded as normal text.
- Sequences that remain open will be automatically closed at the end.
- The character ] will be printed normally if it is not used as closing for a formatting command.
- The character [ can be printed using the empty formatting command [].
- Parameters follow the command type after any single character (preferably space " ", colon ":" or
  equal "=" for the sake of readability).
- The c command does not support alpha, because of a conflict within AprilUI that would cause
  alpha-animated text to be cached for every possible alpha value (0-255) and rendered very slowly.
- Nesting "icon" tags is not allowed.
- When using b and specifying only a border-thickness multiplier, the comma character "," is still
  mandatory.
- When using [l] for italic, keep in mind that there is no way to insert non-italic text within
  italic text.
