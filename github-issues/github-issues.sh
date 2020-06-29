#!/bin/bash

curl -s -i https://api.github.com/repos/$1/$2 | grep open_issues_count | sed 's/[^0-9]*//g'
