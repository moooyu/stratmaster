#!/bin/bash
pushd ../devel > /dev/null
make --quiet
popd > /dev/null
../devel/strat.out < log

