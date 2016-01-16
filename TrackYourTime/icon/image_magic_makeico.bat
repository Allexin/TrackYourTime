::merge separate files into single main.ico
::need image magick tools

convert 16.png 24.png 32.png 48.png 64.png 128.png 256.png ../main.ico
convert 16g.png 24g.png 32g.png 48g.png 64g.png 128g.png 256g.png ../main_gray.ico
pause