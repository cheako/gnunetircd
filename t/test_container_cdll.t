use Test::More tests => 1;
use IPC::Run qw(start);

my $test =
  start( [ 'src/test_container_cdll' ], undef, '>&2' );

$test->finish();
is $test->result(0), 0, 'container cdll ok';

exit 0;
