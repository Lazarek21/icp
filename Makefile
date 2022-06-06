.PHONY=run clean pack all doxygen
ARCHIVE=1_xbrazd21_xsabac02_xloren16.zip
SRC_DIRS=./src
DOC_DIR=./doc
DOC_CONFIG_FILE=doc_config
icp_project:
	$(MAKE) -C $(SRC_DIRS)
clean:
	$(MAKE) -C $(SRC_DIRS) clean

run:
	$(MAKE) -C $(SRC_DIRS) run
pack: 
	zip -r $(ARCHIVE) examples doc src Makefile README.txt
	if [ -d "./TESTDIR" ]; then \
	echo "Directory TESTDIR exists do you want to delete it? [Y/n]"; \
	read line; \
	if  [$$line == "n"]; then exit 1; fi; \
	fi;
	unzip $(ARCHIVE) -d TESTDIR 
	$(MAKE) -C TESTDIR || rm -f $(ARCHIVE);
	if ! [ -f $(ARCHIVE) ]; then echo "Archive wasnt created because some error occured"; fi; 
	rm -fr TESTDIR

doxygen:
	cd $(DOC_DIR); doxygen $(DOC_CONFIG_FILE) 
