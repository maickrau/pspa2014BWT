#!/bin/sh

echo "downloading human genome"
wget http://hgdownload.soe.ucsc.edu/goldenPath/hg19/bigZips/hg19.2bit
echo "downloading twoBitToFa"
wget http://hgdownload.soe.ucsc.edu/admin/exe/linux.x86_64/twoBitToFa
echo "converting genome to .fa"
chmod u+x ./twoBitToFa
./twoBitToFa hg19.2bit hg19.fa
echo "making 10 Mb genome"
tr -cd 'ATCGN' < hg19.fa | head -c 10000000 > genome10
echo "making 50 Mb genome"
tr -cd 'ATCGN' < hg19.fa | head -c 50000000 > genome50
echo "making 100 Mb genome"
tr -cd 'ATCGN' < hg19.fa | head -c 100000000 > genome100
echo "making 250 Mb genome"
tr -cd 'ATCGN' < hg19.fa | head -c 250000000 > genome250
echo "making 500 Mb genome"
tr -cd 'ATCGN' < hg19.fa | head -c 500000000 > genome500
echo "removing big genome files"
rm hg19.fa
rm hg19.2bit
echo "removing twoBitToFa"
rm twoBitToFa
echo "downloading wikiquote dump"
wget dumps.wikimedia.org/enwikiquote/20140208/enwikiquote-20140208-pages-articles-multistream.xml.bz2
echo "unpacking wikiquote dump"
bunzip2 enwikiquote-20140208-pages-articles-multistream.xml.bz2
mv enwikiquote-20140208-pages-articles-multistream.xml wikiquote
echo "making 10 Mb quotes"
tr -d '\0' < wikiquote | head -c 10000000 > quote10
echo "making 50 Mb quotes"
tr -d '\0' < wikiquote | head -c 50000000 > quote50
echo "making 100 Mb quotes"
tr -d '\0' < wikiquote | head -c 100000000 > quote100
echo "making 250 Mb quotes"
tr -d '\0' < wikiquote | head -c 250000000 > quote250
echo "removing big wikiquote file"
rm wikiquote
echo "creating 10 Mb random file"
tr -d '\0' < /dev/urandom | head -c 10000000 > random10
echo "creating 50 Mb random file"
tr -d '\0' < /dev/urandom | head -c 50000000 > random50
echo "creating 100 Mb random file"
tr -d '\0' < /dev/urandom | head -c 100000000 > random100
echo "creating 10 Mb repeating 'a' file"
seq -sa 10000001 | tr -d '\n[:digit:]' > repeat10
echo "creating 50 Mb repeating 'a' file"
seq -sa 50000001 | tr -d '\n[:digit:]' > repeat50
echo "creating 100 Mb repeating 'a' file"
seq -sa 100000001 | tr -d '\n[:digit:]' > repeat100
