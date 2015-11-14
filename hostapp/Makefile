all: synergy-hw

synergy-hw: synergy-hw.c keytables.h uSynergy.c uSynergy.h
	gcc -o synergy-hw synergy-hw.c uSynergy.c -lusb-1.0

clean:
	rm -f *.o synergy-hw
