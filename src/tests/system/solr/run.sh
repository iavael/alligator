#!/bin/sh
VERSION=8.5.2

cd ../../../external/
ls solr-$VERSION.tgz || wget https://downloads.apache.org/lucene/solr/$VERSION/solr-$VERSION.tgz
ls solr-$VERSION || tar xfvz solr-$VERSION.tgz
useradd solr

cd solr-$VERSION
./bin/init.d/solr start

curl 'http://localhost:8983/solr/admin/metrics' | jq
