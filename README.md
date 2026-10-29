# C Web Server

Inspired by [this](https://bruinsslot.jp/articles/simple-http-webserver-in-c/) blog post about building a simple HTTP/1.0 web server in C. I expanded upon the original example a bit by implementing actual request parsing and method handling, static file serving, and concurrency.

# References

- [Making a simple HTTP webserver in C](https://bruinsslot.jp/articles/simple-http-webserver-in-c/)
- [RFC 1945: Hypertext Transfer Protocol -- HTTP/1.0](https://www.rfc-editor.org/info/rfc1945/)
- [RFC 9112: STD 99: HTTP/1.1](https://www.rfc-editor.org/info/rfc9112/)
- [Evolution of HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Evolution_of_HTTP)

# Run

```shell
./build.sh
# if 'zsh: permission denied' error:
# chmod +x ./build.sh
bin/server1.0
```
