set path=(  .                       \
        ./bin                       \
        ~/gzip/bin                  \
        ~/swig/bin                  \
        ~/php/bin                   \
        ~/automake/bin              \
        ~/m4/bin                    \
        ~/autoconf/bin              \
        ~/libtool/bin               \
        /usr/local/teTeX/bin        \
        /opt/SUNWspro/bin           \
        /opt/old/SUNWspro/bin       \
        /usr/lang/SUNWspro/bin      \
        /usr/bin                    \
        /usr/X11R5/bin              \
        /usr/sbin                   \
        /usr/openwin/bin            \
        /usr/ccs/bin                \
        /usr/lang/C++_2.1           \
        /opt/jdk1.2.2/bin           \
        /usr/local/bin              \
        /usr/ucb                    \
        /bin                        \
        /usr/bin/X11                \
        /usr/contrib/bin            \
        /etc /user1/local/bin/X11   \
        /user1/lang/C++_3.0.2       \
        /usr/etc                    \
        /usr/local/gnu/bin          \
        /user1/local/gnu/bin        \
        /usr/lang/ACC               \
        /user3/project/unisql3.1/bin \
      )

setenv MANPATH /user2/tykim/vim/man:/opt/SUNWspro/man:/usr/lang/SUNWspro/man:/usr/man:/usr/local/man:/usr/local/gnu/man:/user1/X11R5/man:/user1/X11/man:/usr/lang/ACC/man:/usr/openwin/man:/usr/man

if( `hostname` == "handel" || `hostname` == "organ" || `hostname` == "flute") then
setenv LD_LIBRARY_PATH /opt/SUNWspro/lib:/usr/X11R5/lib:/usr/openwin/lib:/usr/local/lib:/usr/lib:/usr/ucblib:.
else
setenv LD_LIBRARY_PATH /usr/lang/SUNWspro/lib:/user1/x11r5/lib:/usr/openwin/lib:/user1/home/tykim/php/lib:/user1/home/tykim/swig/lib:/usr/local/lib:/usr/lib:/usr/ucblib:.
endif


#setenv LANG en_US

#
# For Netscape.
#
setenv  XAPPLRESDIR      /usr/local/bin

#
# For Java.
#
#set path=($path /house1/install/JDK1.0.2/java/bin) 
#setenv CLASSPATH .

limit   coredumpsize    0
umask   077
set history=1000
set     ignoreeof
set     noclobber
set     notify

# complete file name.
set     filec       

set     nobeep      
set     lineedit
set     time=100
# set     prompt="[`hostname`,$USER] `dirs`> "
set     prompt = "`hostname` "$PWD" \! >"
set     autologout=0

setenv  TERM    vt100
stty    erase   
stty    kill    
stty    cs8 -istrip -parenb echo echoe

source ~/.alias

# editor
setenv EDITOR helvis
setenv VISUAL helvis

#stty pass8
#stty erase 
#stty kill 
#stty intr 
#stty susp 

set term=vt100
a setppt 'set prompt = "`hostname` "$PWD" \! >"'
# a setppt 'set prompt="[`hostname`,$USER] `dirs`> "'
a cd 'chdir \!* ; setppt'

# For python
# PYTHON
setenv PYTHONPATH .:/usr/local/lib/python2.3:/usr/local/lib/python2.3/tkinter:/proj10/library/Release3.0/server/OOSQL/bin:/nlib/DCOracle/DCOracle

source ~/.odysseus_setup
# For Odysseus
# source ~/Odysseus/setup
