use Test::More tests => 18;
use IPC::Run qw(start);

my $ihlt =
  start( [ 'src/gnunetircd' ], undef, '>&2' );
END { $ihlt->signal(9); }

sleep 3;
use IO::Socket::INET;

# create a connecting socket
my @sockets;
for ( 1 .. 3 ) {
    my $ctr = 0;
    do {
        diag "Connection $_ try $ctr: $!"
          if ( $ctr != 0 );
        $sockets[$_] = new IO::Socket::INET(
            PeerHost => '127.0.0.1',
            PeerPort => '6677',
            Proto    => 'tcp',
        );
      } while ( !$socket[$_]
        && $_ == 1
        && $! == $!{ECONNREFUSED}
        && $ctr++ < 4 );
    unless ( $sockets[$_] ) {
        fail "$_: $ctr: cannot connect to the server $!\n";
        $ihlt->kill_kill;
        die;
    }
    pass "$_: connected to the server";
}

my $response = "";

# data to send to a server
$sockets[1]->autoflush(1);
is $sockets[1]->send("ping :Hello World!\r\n"), 20, 'full ping';
$sockets[1]->recv( $response, 1024 );
is $response, ":gnunetircd PONG gnunetircd :Hello World!\r\n", 'full pong';

$sockets[2]->autoflush(1);
is $sockets[2]->send("ping Hello World!\r\n"), 19, 'word ping';
$sockets[2]->recv( $response, 1024 );
is $response, ":gnunetircd PONG gnunetircd Hello\r\n", 'word pong';

$sockets[3]->autoflush(1);
is $sockets[3]->send("ping\r\n"), 6, 'empty ping';
$sockets[3]->recv( $response, 1024 );
is $response, ":gnunetircd PONG gnunetircd\r\n", 'empty pong';

is $sockets[1]->send("quit\r\n"), 6, 'one quit send';
$sockets[1]->recv( $response, 1024 );
is $response, "ERROR :Closing Link: * (Quit: )\r\n", 'one quit recv';

is $sockets[2]->send("quit :Hello World!\r\n"), 20, 'two quit send';
$sockets[2]->recv( $response, 1024 );
is $response, "ERROR :Closing Link: * (Quit: )\r\n", 'two quit recv';

is $sockets[3]->send("quit :Hello World!\r\n"), 20, 'three quit send';
$sockets[3]->recv( $response, 1024 );
is $response, "ERROR :Closing Link: * (Quit: )\r\n", 'three quit recv';

ok $sockets[$_]->close(), "$_: closed" for ( 1 .. 3 );

$ihlt->signal(9);
END { }
$ihlt->finish();

exit 0;