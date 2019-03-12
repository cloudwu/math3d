LUAINC=-I /usr/local/include
LUALIB=-L /usr/local/bin -llua53
GLM_INC = -I glm
ODIR = o
CFLAGS = -O2 -Wall

all : math3d.dll

$(ODIR)/linalg.o : linalg.c | $(ODIR)
	$(CC) -c $(CFLAGS) -o $@ $^ $(LUAINC) $(GLM_INC)

$(ODIR)/math3d.o : math3d.cpp | $(ODIR)
	$(CXX) -c $(CFLAGS) -Wno-char-subscripts -o $@ $^ $(LUAINC) $(GLM_INC)

$(ODIR)/mathbaselib.o : mathbaselib.cpp | $(ODIR)
	$(CXX) -c $(CFLAGS) -o $@ $^ $(LUAINC) $(GLM_INC)

math3d.dll : $(ODIR)/linalg.o $(ODIR)/math3d.o $(ODIR)/mathbaselib.o
	$(CXX) --shared $(CFLAGS) -o $@ $^ -lstdc++ $(LUALIB)

$(ODIR) :
	mkdir -p $@

clean :
	rm -rf $(ODIR) *.dll
