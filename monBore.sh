#!/bin/bash

ADSLLOGFILE=/var/log/bore.log

DATESTAMP=`date +"%m/%d/%y %H:%M:%S"`

echo -n "${DATESTAMP}" >> ${ADSLLOGFILE}

CURVAL=`/home/mark/buildhome/envData/fetch`

echo -n ", ${CURVAL}" >> ${ADSLLOGFILE}

echo "" >> ${ADSLLOGFILE}


