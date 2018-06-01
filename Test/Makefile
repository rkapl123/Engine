
inc = -I../Wrap -I../QuantExt -I../QuantLib -I${BOOST}

libs = \
  ../Wrap/qlw/.libs/libWrap.a \
  ../QuantExt/qle/.libs/libQuantExt.a \
  ../QuantLib/ql/.libs/libQuantLib.a \
  ${BOOST}/stage/lib/libboost_date_time.a

cc = g++ -Wall -O2 -std=c++11 ${inc} ${libs}

targets = test simulation

all: ${targets}

clean:
	rm -f ${targets} *.o

test: test.cpp ${libs}
	${cc} $< -o $@

simulation: simulation.cpp ${libs}
	${cc} $< -o $@
