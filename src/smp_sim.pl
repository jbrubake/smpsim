#!/usr/bin/perl

die "Usage: smp_sim <n> <delay>\n" if($#ARGV < 1);

$n = $ARGV[0];
$delay = $ARGV[1];
$time = 0;

$buffer[0]{opcode} = "INVALID";

@buffer = get_instruction_block(@buffer);
@buffer = get_instruction_block(@buffer);

print "$buffer[0]{opcode}\n";

sub get_instruction_block{
    my @buffer = @_;
    my $lower = 0;
    my $upper = 15;

    if($buffer[15]{opcode} ne "INVALID"){
        $lower = 16;
        $upperl = 31;
    }

    for($i = $lower; $i < $upper + 1; $i++){
        <STDIN>;
        print;
        /(.*)\w*R(\d+)\w*,\w*R(\d+)\w*,\w*R(\d+)/;
        $buffer[$i]{opcode} = $1;
        $buffer[$i]{operands}[0] = $2;
        $buffer[$i]{operands}[1] = $3;
        $buffer[$i]{operands}[2] = $4;
    }

    return(@buffer);
}
