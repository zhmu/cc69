OBJS=		photoviewer.o image.o imagelibrary.o stringlibrary.o texture.o messagewindow.o game.o events.o app.o \
		gateware.o menu.o clock.o particle.o fireworks.o random.o
CPPFLAGS=	`sdl-config --cflags` -g -O3
LDFLAGS=	-lSDL -lSDL_image -lSDL_ttf -lSDL_mixer -lGL -lGLU -lboost_system -lboost_filesystem -lboost_thread

cc69:		$(OBJS)
		$(CXX) -o cc69 $(OBJS) $(LDFLAGS)

clean:
		rm -f cc69 $(OBJS)
