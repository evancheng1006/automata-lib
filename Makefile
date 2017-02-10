all:
	g++ main.cc cfg.cc -o main.exe -Wall
clean:
	rm main.exe
run:
	./main.exe
