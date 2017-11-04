fn='./test_modellist'
iteration=5000
folder=5

for (( j=0; j<${iteration}; j=j+1 )); do
	>&2 echo "$j"
	echo -n "$j: "

	# create modellist
	for (( i=1; i<=${folder}; i=i+1 )); do
		path="./record/0${i}/iteration_${j}"
		echo $path >> $fn
	done

	# test and print accuracy
	# cat $fn
	./test ${fn} testing_data1.txt test_result.txt
	python3 test_acc.py test_result.txt
	rm $fn
done
