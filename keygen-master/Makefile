CC = clang
CFLAGS = -Iinclude -Wall -Wextra -O1 -g -fsanitize=address -fno-omit-frame-pointer
#CFLAGS = -Iinclude -Wall -Wextra -O0 -g  -fno-omit-frame-pointer -fsanitize=address

bin/test_bigint: out/test_bigint.o out/test_utils.o bin/libbigint.a 
	$(CC) $(CFLAGS) $^ -o $@

bin/tc_mul: out/tc_mul.o bin/libbigint.a 
	$(CC) $(CFLAGS) $^ -o $@

bin/gs_mul: out/gs_mul.o bin/libbigint.a 
	$(CC) $(CFLAGS) $^ -o $@

bin/kara_mul: out/kara_mul.o bin/libbigint.a 
	$(CC) $(CFLAGS) $^ -o $@

bin/keygen: out/rsa.o out/base64.o out/keygen.o out/rsa_private_key.o bin/libbigint.a
	$(CC) $(CFLAGS) $^ -o $@

bin/libbigint.a: out/bigint.o out/random.o
	ar -cr $@ $^

out/%.o: src-given/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

out/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

out/%.o: tests/%.c 
	$(CC) -c $(CFLAGS) $^ -o $@


clean:
	rm -f out/* bin/*
