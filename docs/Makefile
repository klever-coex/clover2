REPO_DIR = $(shell pwd)

SHELL := /bin/bash

SOURCE     	= source
OUT       	= build
PYTHON		:= python3
BUILD      	= $(PYTHON) -m sphinx
OPTS       	=-c . -W --verbose

html-ru:
	@$(BUILD) -b html -D language='ru' $(OPTS) ./ru ./build/html/ru

html-en:
	@$(BUILD) -b html -D language='en' $(OPTS) ./en ./build/html/en

html: html-ru html-en

pdf-ru:
	@$(BUILD) -b simplepdf -D language='ru' $(OPTS) ./ru ./build/pdf/ru

pdf-en:
	@$(BUILD) -b simplepdf -D language='en' $(OPTS) ./en ./build/html/en

pdf: pdf-ru pdf-en

clean:
	rm -rf build

host: html
	$(PYTHON) -m http.server -d $(REPO_DIR)/build/html/ru

check-images:
	python3 ./image_cleaner.py --dry-run ./assets

optimize-images:
	python3 ./image_cleaner.py -v ./assets
