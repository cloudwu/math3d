LUAINC=-I /usr/local/include
LUALIB=-L /usr/local/bin -llua54
GLM_INC = -I glm
ODIR = o
CFLAGS = -O2 -Wall
OUTPUT=./

.PHONY : all test

all : $(OUTPUT)math3d.dll
test : mathid_test.exe

$(ODIR)/mathid.o : mathid.c | $(ODIR)
	$(CC) -c $(CFLAGS) -o $@ $^ $(LUAINC)

$(ODIR)/math3d.o : math3d.c | $(ODIR)
	$(CC) -c $(CFLAGS) -o $@ $^ $(LUAINC)

$(ODIR)/math3dfunc.o : math3dfunc.cpp | $(ODIR)
	$(CXX) -c $(CFLAGS) -Wno-char-subscripts -o $@ $^ $(GLM_INC)

$(ODIR)/mathadapter.o : mathadapter.c | $(ODIR)
	$(CC) -c $(CFLAGS) -o $@ $^ $(LUAINC)

$(ODIR)/testadapter.o : testadapter.c | $(ODIR)
	$(CC) -c $(CFLAGS) -o $@ $^ $(LUAINC)

$(OUTPUT)math3d.dll : $(ODIR)/mathid.o $(ODIR)/math3d.o $(ODIR)/math3dfunc.o $(ODIR)/mathadapter.o $(ODIR)/testadapter.o
	$(CXX) --shared $(CFLAGS) -o $@ $^ -lstdc++ $(LUALIB)

$(ODIR) :
	mkdir -p $@

clean :
	rm -rf $(ODIR) *.dll *.exe

mathid_test.exe : mathid.c
	$(CC) $(CFLAGS) -DTEST_MATHID -o $@ $^