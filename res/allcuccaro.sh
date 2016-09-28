for i in `seq 4 20`;
do
	x=`./recycle cucc $i 0 0`
	echo $i $x
done

echo "--------"

for i in `seq 4 20`;
do
	x=`./recycle cucc $i 0 1`
	echo $i $x
done

echo "--------"

for i in `seq 4 20`;
do
	x=`./recycle cucc $i 1 0`
	echo $i $x
done

echo "--------"

for i in `seq 4 20`;
do
	x=`./recycle cucc $i 1 1`
	echo $i $x
done
