for i in `find . -type f | grep -v "\/.svn\/" | grep -v "\.keep$" | grep -v "~$" | grep "\.[ch]$"`
do
diff -u $i ../contrib/uclibc/$i
done > diff.txt

find . -type f | grep -v "\/.svn\/" | grep -v "\.keep$" | grep -v "~$" | grep "\.[ch]$" > diff.lst
