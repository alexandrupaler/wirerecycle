FILES=../*.real
for f in $FILES
do
	./a.out $f >> "raw/$f.raw"
done
