#!/bin/bash

# Print colored text, with guidance about the terminal's color capabilities
# Adapted from:
# - 256color2.pl by Todd Larason <jtl@molehill.org>
# - https://github.com/termstandard/colors
# By Andrew Schulman <andrex.e.schulman@gmail.com>

# System colors (range 0-15)

echo System colors:
for ((color = 0; color < 8; ++color)) ; do
	printf "\033[48;5;${color}m  "
done
printf "\033[0m\n"
for ((color = 8; $color < 16; ++color)) ; do
	printf "\033[48;5;${color}m  "
done
printf "\033[0m\n\n"

# Color cube (range 16-231)

echo Color cube, 6x6x6:
for ((g = 0; g < 6; ++g)) ; do
	for ((r = 0; r < 6; ++r)) ; do
		for ((b = 0; b < 6; ++b)) ; do
			color=$(( 16 + 36*r + 6*g + b ))
			printf "\033[48;5;${color}m  "
		done
		printf "\033[0m "
	done
	echo
done
echo

# Grayscale ramp (range 232-255)

echo Grayscale ramp:
for ((color = 232; color < 256; ++color)) ; do
	printf "\033[48;5;${color}m  "
done
printf "\033[0m\n\n"

# Truecolor ramp

for n in $(tput cols) $COLUMNS 80 ; do
	ncols=$n
	break
done

echo Truecolor ramp:
for ((col = 0; col < ncols; ++col)) ; do
	r=$(( 255 - ( 255 * col / ncols ) ))
	g=$(( 510 * col / ncols )) ; if (( g > 255 )) ; then g=$(( 510 - g )) ; fi
	b=$(( 255 * col / ncols ))
	printf "\033[48;2;%d;%d;%dm" $r $g $b
	printf "\033[38;2;%d;%d;%dm" $(( 255 - r )) $(( 255 - g )) $(( 255 - b ))
	printf -- "-\033[0m"
done
echo

# Interpretation

ncols=$(printf "%3d" $ncols)
cat <<END

If your terminal supports
this many colors:                  8     16     256  24-bit

Then you'll see about this many
distinct shades of color above:

System colors ..................   8     16    8-16      16
Color cube (each square) .......   3     10      36      36
Greyscale ramp (incl. black) ...   1      4      25      25
Truecolor ramp .................   1      1    1-64     ${ncols}

See /usr/share/doc/screen/README.msys2 for information about terminal color
support on MSYS2.
END
