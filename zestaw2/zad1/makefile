CC = gcc -std=c11


tests:
	$(CC) -o res main.c -O2
	#compiled

	#generate
	for l in 4 512 4096 8192; \
	do \
		for a in 3000 10000 60000 120000; \
		do \
			echo "creating file with $$a records of size $$l"; \
			./res generate test_file $$a $$l; \
			rm test_file; \
		done; \
	done

	#copy test
	for l in 4 512 4096 8192; \
	do \
		for a in 3000 10000 60000 120000; \
		do \
			echo "creating file with $$a records of size $$l"; \
			./res generate test_file $$a $$l; \
			for m in sys lib; \
			do \
				cp test_file test_file_temp; \
				echo "$$m copy file with $$a records of size $$l"; \
				./res copy test_file_temp test_copy $$a $$l $$m; \
				rm test_file_temp; \
				rm test_copy; \
			done; \
			rm test_file; \
		done; \
	done


	#sort test
	for l in 4 512 4096 8192; \
	do \
		for a in 1000 3000 10000; \
		do \
			echo "creating file with $$a records of size $$l"; \
			./res generate test_file $$a $$l; \
			for m in sys lib; \
			do \
				cp test_file test_file_temp; \
				echo "$$m sorting file with $$a records of size $$l"; \
				./res sort test_file_temp $$a $$l $$m; \
				rm test_file_temp; \
			done; \
			rm test_file; \
		done; \
	done

