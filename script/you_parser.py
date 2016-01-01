#!/usr/bin/env python
# -*- coding: utf-8 -*-


import BaseHTTPServer
import SocketServer
import json
import re
import time
from random import random
import urllib2
from urlparse import urlparse
import base64

fake_headers = {
    'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
    'Accept-Charset': 'UTF-8,*;q=0.5',
    'Accept-Encoding': 'gzip,deflate,sdch',
    'Accept-Language': 'en-US,en;q=0.8',
    'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64; rv:13.0) Gecko/20100101 Firefox/13.0'
}


# DEPRECATED in favor of match1()
def r1(pattern, text):
    m = re.search(pattern, text)
    if m:
        return m.group(1)


# DEPRECATED in favor of get_content()
def get_response(url, faker = False):
    print(url)
    print()
    if faker:
        response = urllib2.urlopen(request.Request(url, headers = fake_headers), None)
    else:
        response = urllib2.urlopen(url)

    data = response.read()
    if response.info().get('Content-Encoding') == 'gzip':
        data = ungzip(data)
    elif response.info().get('Content-Encoding') == 'deflate':
        data = undeflate(data)
    response.data = data
    return response

# DEPRECATED in favor of get_content()
def get_html(url, encoding = None, faker = False):
    content = get_response(url, faker).data
    return content

# DEPRECATED in favor of get_content()
def get_decoded_html(url, faker = False):
    response = get_response(url, faker)
    data = response.data
    charset = r1(r'charset=([\w-]+)', response.headers['content-type'])
    if charset:
        return data.decode(charset, 'ignore')
    else:
        return data


class Iqiyi:

    def __init__(self, url):
        pass

class Sohu:

    def __init__(self, url):
        self.url = url

    def real_url(self, host, vid, tvid, new, clipURL, ck):
        url = 'http://'+host+'/?prot=9&prod=flash&pt=1&file='+clipURL+'&new='+new +'&key='+ ck+'&vid='+str(vid)+'&uid='+str(int(time.time()*1000))+'&t='+str(random())+'&rb=1'
        return json.loads(get_html(url))['url']

    def extract(self, meta, quality):
        print(meta)
        print(quality)
        if re.match(r'http://share.vrs.sohu.com', self.url):
            vid = r1('id=(\d+)', url)
        else:
            html = get_html(self.url)
            vid = r1(r'\Wvid\s*[\:=]\s*[\'"]?(\d+)[\'"]?', html)
        assert vid

        infos = []
        if re.match(r'http://tv.sohu.com/', self.url):
            info = json.loads(get_decoded_html('http://hot.vrs.sohu.com/vrs_flash.action?vid=%s' % vid))
            for qtyp in ["oriVid","superVid","highVid" ,"norVid","relativeId"]:
                hqvid = info['data'][qtyp]
                if hqvid != 0 and hqvid != vid :
                    info = json.loads(get_decoded_html('http://hot.vrs.sohu.com/vrs_flash.action?vid=%s' % hqvid))

                    host = info['allot']
                    prot = info['prot']
                    tvid = info['tvid']
                    urls = []
                    data = info['data']
                    title = data['tvName']
                    size = sum(data['clipsBytes'])
                    assert len(data['clipsURL']) == len(data['clipsBytes']) == len(data['su'])
                    if meta:
                        item = {
                            "quality": qtyp,
                            "totalDuration": data["totalDuration"],
                            "totalBytes": data["totalBytes"],
                            "clipsBytes": data["clipsBytes"],
                            "clipsDuration": data["clipsDuration"]
                            }
                        infos.append(item)
                        continue

                    if quality == qtyp:
                        for new,clip,ck, in zip(data['su'], data['clipsURL'], data['ck']):
                            clipURL = urlparse(clip).path
                            urls.append(self.real_url(host,hqvid,tvid,new,clipURL,ck))

                        item = {
                            "urls": urls,
                            "quality": qtyp,
                            "totalDuration": data["totalDuration"],
                            "totalBytes": data["totalBytes"],
                            "clipsBytes": data["clipsBytes"],
                            "clipsDuration": data["clipsDuration"]
                            }
                        return item


        return infos

SITES = {
    'sohu': Sohu,
    "iqiyi": Iqiyi
    }

def url_to_module(url):
    try:
        video_host = r1(r'https?://([^/]+)/', url)
        video_url = r1(r'https?://[^/]+(.*)', url)
        assert video_host and video_url
    except:
        url = google_search(url)
        video_host = r1(r'https?://([^/]+)/', url)
        video_url = r1(r'https?://[^/]+(.*)', url)

    if video_host.endswith('.com.cn'):
        video_host = video_host[:-3]
    domain = r1(r'(\.[^.]+\.[^.]+)$', video_host) or video_host
    assert domain, 'unsupported url: ' + url

    k = r1(r'([^.]+)', domain)
    if k in SITES:
        return SITES[k]
    return None


class YouParserHTTPRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):

    server_version = "YouParserHTTP/1"

    def response_err_msg(self, code, msg = 'error'):
        s = json.dumps({'err': code, 'msg': msg})
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.send_header("Content-Length", len(s))
        self.end_headers()
        self.wfile.write(s)

    def response_success(self, data):
        s = json.dumps({'err': 0, 'msg': '', 'data': data})
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.send_header("Content-Length", len(s))
        self.end_headers()
        self.wfile.write(s)

    def response_media(self, site_class, url, params):
        if not site_class:
            return self.response_err_msg(1, 'not support')
        site = site_class(url)
        data = site.extract(params.get('meta', None), params.get('quality', None))
        self.response_success(data)

    def do_response(self):
        path, query = self.path.split('?', 1)
        d = dict(tuple([x.split('=', 1) for x in query.split('&')]))
        if path == "/media":
            url = base64.b64decode(d['url'])
            site_class = url_to_module(url)
            self.response_media(site_class, url, d)
        else:
            self.response_error()

    def do_GET(self):
        try:
            self.do_response()
        except:
            self.response_err_msg(500, 'server error')


# curl http://127.0.0.1:8909/media?url=aHR0cDovL3R2LnNvaHUuY29tLzIwMTUxMjIxL240MzIxMjY5MTcuc2h0bWw=&meta=true
# curl http://127.0.0.1:8909/media?url=aHR0cDovL3R2LnNvaHUuY29tLzIwMTUxMjIxL240MzIxMjY5MTcuc2h0bWw=&quality=highVid
def start_server(port):
    handler = YouParserHTTPRequestHandler
    httpd = SocketServer.TCPServer(("127.0.0.1", port), handler)
    httpd.serve_forever()


if __name__ == '__main__':
    start_server(9099)
    s = "http://tv.sohu.com/20151221/n432126917.shtml"
    en = base64.b64encode(s)
    # aHR0cDovL3R2LnNvaHUuY29tLzIwMTUxMjIxL240MzIxMjY5MTcuc2h0bWw=
    de = base64.b64decode(en)
    print(en)
    print(de)
    # start_server(9809)
