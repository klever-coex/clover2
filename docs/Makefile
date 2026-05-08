REPO_DIR = $(shell pwd)

SHELL := /bin/bash

SOURCE     	= source
OUT       	= build
PYTHON		:= python3
BUILD      	= $(PYTHON) -m sphinx
OPTS       	=-c . -W --verbose

html_ru:
	@$(BUILD) -b html -D language='ru' $(OPTS) ./ru ./build/html/ru

html_en:
	@$(BUILD) -b html -D language='en' $(OPTS) ./en ./build/html/en

html: html_ru html_en

clean:
	rm -rf build

check_images:
	python3 ./image_cleaner.py --dry-run ./assets

optimize_images:
	python3 ./image_cleaner.py -v ./assets
