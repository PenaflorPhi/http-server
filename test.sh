#!/bin/bash


(sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 8080 &
(sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 8080 &
(sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 8080 &
