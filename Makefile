CC = g++

re: re.cpp
	$(CC) re.cpp -o re -Wall -Wextra -pedantic -std=c++11
