FILES=circs/raw/*.raw
for f in $FILES
do
	x=`../bin/recycle $f 0 0 0`
	echo $f $x
done

echo "--------"

for f in $FILES
do
	x=`../bin/recycle $f 0 0 1`
	echo $f $x
done
