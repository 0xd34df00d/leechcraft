#!/bin/sh
#############################################################
# TEX2IM:   Converts LaTeX formulas to pixel graphics which #
#           can be easily included in Text-Processors like  #
#           M$ or Staroffice.                               #
#                                                           #
# Required software: latex, convert (image magic)           #
# to get color, the latex color package is required         #
#############################################################
# Version 1.8  (http://www.nought.de/tex2im.html)           #
# published under the GNU public licence (GPL)              #
# (c) 14. May 2004 by Andreas Reigber                   #
# Email: anderl@nought.de                                   #
#############################################################

#
# Default values
#

resolution="150x150"
format="png"
color1="white"
color2="black"
trans=1
noformula=0
aa=1
extra_header="$HOME/.tex2im_header"

if [ -f ~/.tex2imrc ]; then
	. ~/.tex2imrc
fi

OPTERR=0

if [ $# -lt 1 ]; then
	echo "Usage: `basename $0` [options] file.tex, for help give option -h" 1>&2
	exit 1
fi

while getopts hanzb:t:f:o:r:vx: Optionen; do
	case $Optionen in
		h) echo "tex2im [options] latex_expression

The content of input file should be _plain_ latex mathmode code!
Alternatively, a string containing the latex code can be specified.

Options:
-v         show version
-h         show help
-a         change status of antialiasing
           default is on for normal mode and
			  off for transparent mode
-o file    specifies output filename,
           default is inputfile with new extension
-f expr    specifies output format,
           possible examples: gif, jpg, tif......
           all formates supported by 'convert' should work,
           default: png
-r expr    specifies desired resolution in dpi,
           possible examples: 100x100, 300x300, 200x150,
           default is 150x150
-b expr    specifies the background color
           default: white
-t expr    specifies the text color
           default: black
-n         no-formula mode (do not wrap in eqnarray* environment)
           default: off
-z         transparent background
           default: off
-x file    file containing extra header lines.
           default: ~/.tex2im_header"
			exit 0 ;;
		v) echo "TEX2IM Version 1.8"
			exit 0 ;;
		r) resolution=$OPTARG;;
		o) outfile=$OPTARG;;
		z) trans=1
		   aa=0;;
		a) if [ $aa -eq 0 ]; then
				aa=1
			else
				aa=0
			fi;;
		n) noformula=1;;
		b) color1=$OPTARG;;
		t) color2=$OPTARG;;
		f) format=$OPTARG;;
      x) extra_header=$OPTARG;;
	esac
done

#
# Generate temporary directory
#

if test -n "`type -p mktemp`" ; then
	tmpdir="`mktemp -d /tmp/tex2imXXXXXX`"
else
	tmpdir=/tmp/tex2im$$
	if [ -e $tmpdir ] ; then
		echo "$0: Temporary directory $tmpdir already exists." 1>&2
		exit 1
	fi
	mkdir $tmpdir
fi
homedir="`pwd`" || exit 1

#
# Names for input and output files
#

while [ $OPTIND -le $# ]
do

eval infile=\$${OPTIND}

if [ -z $outfile ]; then
	if [ -e "$infile" ]; then
		base=`basename ${infile} .tex` ;
		outfile=${base}.$format
	else
		outfile=out.$format
	fi
fi

#
# Here we go
#

(
cat << ENDHEADER1
\documentclass[12pt]{article}
\usepackage{color}
\usepackage{amsmath,latexsym,amsfonts,amssymb,ulem}
\usepackage[dvips]{graphicx}
\pagestyle{empty}
ENDHEADER1
) > $tmpdir/out.tex

#
# Do we have a file containing extra files to include into the header?
#

if [ -f $extra_header ]; then
	(
	cat $extra_header
	) >> $tmpdir/out.tex
fi

if [ $noformula -eq 1 ]; then
(
cat << ENDHEADER2
\pagecolor{$color1}
\begin{document}
{\color{$color2}
ENDHEADER2
) >> $tmpdir/out.tex
else
(
cat << ENDHEADER2
\pagecolor{$color1}
\begin{document}
{\color{$color2}
\begin{eqnarray*}
ENDHEADER2
) >> $tmpdir/out.tex
fi

# Kopete does not need to parse the content of a file.
#if [ -e "$infile" ]; then
#	cat $infile >> $tmpdir/out.tex
#else
	printf '%s' "$infile" >> $tmpdir/out.tex
#fi

if [ $noformula -eq 1 ]; then
(
cat << ENDFOOTER
}\end{document}
ENDFOOTER
) >> $tmpdir/out.tex
else
(
cat << ENDFOOTER
\end{eqnarray*}}
\end{document}
ENDFOOTER
) >> $tmpdir/out.tex
fi

cd $tmpdir
for f in $homedir/*.eps; do
    test -f ${f##*/} || ln -s $f . # multi-processing!
done
latex -interaction=batchmode -halt-on-error out.tex > /dev/null
[ ! -e out.dvi ] && exit 1;
cd "$homedir"
dvips -o $tmpdir/out.eps -E $tmpdir/out.dvi 2> /dev/null

#
# Transparent background
#

if [ $trans -eq 1 ]; then
	if [ $aa -eq 1 ]; then
		convert +adjoin -antialias -transparent $color1 -density $resolution $tmpdir/out.eps $tmpdir/out.$format
	else
		convert +adjoin +antialias -transparent $color1 -density $resolution $tmpdir/out.eps $tmpdir/out.$format
	fi
else
	if [ $aa -eq 1 ]; then
		convert +adjoin -antialias -density $resolution $tmpdir/out.eps $tmpdir/out.$format
	else
		convert +adjoin +antialias -density $resolution $tmpdir/out.eps $tmpdir/out.$format
	fi
fi


if [ -e $tmpdir/out.$format ]; then
	mv $tmpdir/out.$format $outfile
else
	mv $tmpdir/out.$format.0 $outfile
fi

OPTIND=$((${OPTIND}+1))
outfile=""
done

#
# Cleanup
#

rm -rf $tmpdir
exit 0
