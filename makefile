# Raspberry Pi
RPIIP		?= 192.168.1.110
RPI_USER	?= pi
RPI_PASS	?= raspberry
RPI_DIR		?= /home/pi/Reflexe/bin

#build
ncurses		= build/ncurses/pi
wiringPi	= build/wiringPi

bin		= bin
lib		= lib
test	= test

# Compileur
PATH_CC	= compileur/bin
CC		= $(PATH_CC)/arm-linux-gnueabihf-gcc
CFLAGS	= -Wall -Wextra -I$(wiringPi)/include/ -I$(ncurses)/include/ -I$(lib)/
LDFLAGS	= -L$(wiringPi)/lib -lwiringPi -L$(ncurses)/lib -lncurses -lpthread


all: install

####################     lib     ####################

$(bin)/7_segment.o: $(lib)/my_7_segment.c $(lib)/my_7_segment.h
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

$(bin)/matrice_btn.o: $(lib)/my_matrice_btn.c $(lib)/my_matrice_btn.h
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

$(bin)/matrice_led.o: $(lib)/my_matrice_led.c $(lib)/my_matrice_led.h
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

$(bin)/ncurses.o: $(lib)/my_ncurses.c $(lib)/my_ncurses.h
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

$(bin)/lcd.o: $(lib)/my_lcd.c $(lib)/my_lcd.h
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

$(bin)/touch.o: $(lib)/my_touch.c $(lib)/my_touch.h
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)


$(bin)/data.o: $(lib)/data.c $(lib)/data.h
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

$(bin)/session.o: $(lib)/session.c $(lib)/session.h
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

#####################################################



####################     test     ####################

$(bin)/7_segment: $(test)/7_segment.c $(bin)/7_segment.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(bin)/matrice_btn: $(test)/matrice_btn.c $(bin)/matrice_btn.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(bin)/matrice_led: $(test)/matrice_led.c $(bin)/matrice_led.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(bin)/ncurses: $(test)/ncurses.c $(bin)/ncurses.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(bin)/lcd: $(test)/lcd.c $(bin)/lcd.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(bin)/touch: $(test)/touch.c $(bin)/touch.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(bin)/main.o: main.c main.h formes.c $(bin)/7_segment.o $(bin)/matrice_btn.o $(bin)/matrice_led.o $(bin)/ncurses.o $(bin)/lcd.o $(bin)/touch.o
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

#####################################################



####################     main     ####################

# jeu en local
$(bin)/main: main.c main.h formes.c $(bin)/7_segment.o $(bin)/matrice_btn.o $(bin)/matrice_led.o $(bin)/ncurses.o $(bin)/lcd.o $(bin)/touch.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(bin)/client: client.c client.h $(bin)/main.o $(bin)/session.o $(bin)/data.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(bin)/serveur: serveur.c serveur.h $(bin)/main.o $(bin)/session.o $(bin)/data.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

#####################################################



####################     install     ####################

install_7_segment: $(bin)/7_segment
	sshpass -p $(RPI_PASS) scp $^ $(RPI_USER)@$(RPIIP):$(RPI_DIR)/test

install_matrice_btn: $(bin)/matrice_btn
	sshpass -p $(RPI_PASS) scp $^ $(RPI_USER)@$(RPIIP):$(RPI_DIR)/test

install_matrice_led: $(bin)/matrice_led
	sshpass -p $(RPI_PASS) scp $^ $(RPI_USER)@$(RPIIP):$(RPI_DIR)/test

install_ncurses: $(bin)/ncurses
	sshpass -p $(RPI_PASS) scp $^ $(RPI_USER)@$(RPIIP):$(RPI_DIR)/test

install_lcd: $(bin)/lcd
	sshpass -p $(RPI_PASS) scp $^ $(RPI_USER)@$(RPIIP):$(RPI_DIR)/test

install_touch: $(bin)/touch
	sshpass -p $(RPI_PASS) scp $^ $(RPI_USER)@$(RPIIP):$(RPI_DIR)/test

install_test: install_7_segment install_matrice_btn install_matrice_led install_ncurses install_lcd install_touch

install_main: $(bin)/main
	sshpass -p $(RPI_PASS) scp $(bin)/main $(RPI_USER)@$(RPIIP):$(RPI_DIR)

install_client: $(bin)/client
	sshpass -p $(RPI_PASS) scp $(bin)/client $(RPI_USER)@$(RPIIP):$(RPI_DIR)

install_serveur: $(bin)/serveur
	sshpass -p $(RPI_PASS) scp $(bin)/serveur $(RPI_USER)@$(RPIIP):$(RPI_DIR)

isntall_local: install_main

install_remote: install_client install_serveur

install_jeu: install_main install_client install_serveur

install_all: install_test install_jeu

#########################################################


clean:
	rm -f $(bin)/*