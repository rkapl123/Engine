
# Simple build all makefile, can be run with multiple jobs "make -j10"
all:
	cd QuantLib; $(MAKE)
	cd QuantExt; $(MAKE)
	cd OREData; $(MAKE)
	cd OREAnalytics; $(MAKE)
	cd App; $(MAKE)
	cd QuantExtPlus; $(MAKE)
	cd OREDataPlus; $(MAKE)
	cd OREPlus; $(MAKE)
	cd OREAnalyticsPlus; $(MAKE)
	cd AppPlus; $(MAKE)
    

# target just to make the libs (and no QL) which is quicker to run
libs:
	cd QuantExt/qle; $(MAKE)
	cd OREData/ored; $(MAKE)
	cd OREAnalytics/orea; $(MAKE)
	cd App; $(MAKE)
	cd QuantExtPlus/qlep; $(MAKE)
	cd OREDataPlus/oredp; $(MAKE)
	cd OREPlus; $(MAKE)
	cd OREAnalyticsPlus/oreap; $(MAKE)
	cd AppPlus; $(MAKE)

clean:
	cd QuantExt; $(MAKE) clean
	cd OREData; $(MAKE) clean
	cd OREAnalytics; $(MAKE) clean
	cd App; $(MAKE) clean
	cd QuantExtPlus; $(MAKE) clean
	cd OREDataPlus; $(MAKE) clean
	cd OREPlus; $(MAKE) clean
	cd OREAnalyticsPlus; $(MAKE) clean
	cd AppPlus; $(MAKE) clean
    
