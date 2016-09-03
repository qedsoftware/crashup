#!/usr/bin/env python
# -----------------------------------------------------------------------------
# Copyright (c) 2016 by Quantitative Engineering Design, Inc.
# ALL RIGHTS RESERVED.
# QED | Contact: William Wu <w@qed.ai>
# -----------------------------------------------------------------------------

import re
import requests

import time


from .private import KIBANA_URL

print(KIBANA_URL)

def _time_int():
    return int(time.time()*1000)

def now():
    return _time_int()

def _payload_for(product = None, version = None, time_gte = None, time_lte = None):
    if time_gte is None:
        time_gte = _time_int() - 3600 * 1000*24
    if time_lte is None:
        time_lte = _time_int() + 10 * 1000
    if product is None:
        product = '*'
    else:
        product = 'raw_crash.ProductName: "%s"' % product
    if version is None:
        version = '*'
    else:
        version = 'raw_crash.Version: "%s"' % version

    payload = {
        "size":500,
        "sort":[{"raw_crash.submitted_timestamp":{"order":"desc","unmapped_type":"boolean"}}],
        "query": {
            "filtered": {
                "query": {"query_string":{
                    "analyze_wildcard":True,
                    "query": product + ' and ' + version
                }},
                "filter": {
                    "bool": {
                        "must":[{"range":{"raw_crash.submitted_timestamp":{"gte":time_gte,"lte":time_lte}}}],
                        "must_not":[]
                    }
                }
            }
        },
        "fields":["*","_source"],
        "fielddata_fields":["processed_crash.completed_datetime","processed_crash.submitted_timestamp","processed_crash.client_crash_date","raw_crash.submitted_timestamp","processed_crash.startedDateTime","processed_crash.started_datetime","processed_crash.build_date","processed_crash.date_processed","processed_crash.completeddatetime"]
    }
    return payload

def query(server=KIBANA_URL, product=None, version=None, time_gte=None, time_lte=None):
    if not re.match('[a-z]+://', server):
      server = 'http://' + server
    r = requests.post(server+"/elasticsearch/socorro_reports/_search",
                      params={"timeout": 0, "ignore_unavailable": True, "preference": _time_int()},
                      json=_payload_for(product=product, version=version, time_gte=time_gte, time_lte=time_lte),
                      headers={"kbn-xsrf-token":"kibana"})
    data = r.json()
    if data['_shards']['successful'] != data['_shards']['total']:
        return None
    else:
        return data['hits']['hits']


def crash_ids_from_timepoint(timepoint, server=KIBANA_URL, product=None, version=None):
    hits = query(server=server, product=product, version=version, time_gte=timepoint, time_lte=now() + 3 * 1000)
    return { h['_source']['crash_id']: h['_source']['raw_crash']['dump_checksums']['upload_file_minidump'] for h in hits }
