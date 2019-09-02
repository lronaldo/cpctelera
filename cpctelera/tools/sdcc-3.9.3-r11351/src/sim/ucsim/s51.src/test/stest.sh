VERS="504 505 606 610 615 620 625 645"

TESTS=$(cat st_list.txt)

rm -f *.csv
echo -n "versions " >st_e.csv
echo -n "versions " >st_st.csv
echo -n "versions " >st_s.csv
for v in $VERS; do
    echo -n ", v${v}" >>st_e.csv
    echo -n ", v${v}" >>st_st.csv
    echo -n ", v${v}" >>st_s.csv
done
echo >>st_e.csv
echo >>st_st.csv
echo >>st_s.csv

for t in $TESTS; do
    for v in $VERS; do
	make -f st${t}.mk clean all
	./st.sh -v $v $t
    done

    (
	echo -n "st${t} "
	for v in $VERS; do
	     echo -n ", $(cat st${t}${v}_e.txt) "
	done
	echo
    ) >st${t}_e.csv
    cat st${t}_e.csv >>st_e.csv
    
    (
	echo -n "st${t} "
	for v in $VERS; do
	     echo -n ", $(cat st${t}${v}_st.txt) "
	done
	echo
    ) >st${t}_st.csv
    cat st${t}_st.csv >>st_st.csv
    
    (
	echo -n "st${t} "
	for v in $VERS; do
	     echo -n ", $(cat st${t}${v}_s.txt) "
	done
	echo
    ) >st${t}_s.csv
    cat st${t}_s.csv >>st_s.csv

done
