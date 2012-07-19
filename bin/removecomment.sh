#! /bin/sed -nf
 
#  How do I use this sed script?
# $ ./script.sed < input.c > output.c
# $ for c in *.c; do script.sed < $c > /tmp/zyzcc.c; /bin/cp -f /tmp/zyzcc.c $c; done

:loop
 
# This line is sufficient to remove C++ comments!
/^\/\// s,.*,,
 
/^$/{
  x
  p
  n
  b loop
}
/^"/{
  :double
  /^$/{
    x
    p
    n
    /^"/b break
    b double
  }
 
  H
  x
  s,\n\(.[^\"]*\).*,\1,
  x
  s,.[^\"]*,,
 
  /^"/b break
  /^\\/{
    H
    x
    s,\n\(.\).*,\1,
    x
    s/.//
  }
  b double
}
 
/^'/{
  :single
  /^$/{
    x
    p
    n
    /^'/b break
    b single
  }
  H
  x
  s,\n\(.[^\']*\).*,\1,
  x
  s,.[^\']*,,
 
  /^'/b break
  /^\\/{
    H
    x
    s,\n\(.\).*,\1,
    x
    s/.//
  }
  b single
}
 
/^\/\*/{
  s/.//
  :ccom
  s,^.[^*]*,,
  /^$/ n
  /^\*\//{
    s/..//
    b loop
  }
  b ccom
}
 
:break
H
x
s,\n\(.[^"'/]*\).*,\1,
x
s/.[^"'/]*//
b loop
