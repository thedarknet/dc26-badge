#!/usr/bin/perl

$eeprom_size = 512;
$TYPE = "EGGPLANT";
$VERSION = "1.0.0";
$SERIALNUM = "";
@OPTIONS = (0,0,0,0,0,0,0,0,0);
$NAME = "";
@names =("#EGGPLANT",
         "Unnamed-EP",
         "SteveEggplnt",
         ":eggplant:",
         "DstryrofEggs",
         "UnnamedEggie",
         "Just-An-EP",
         "GULOFAN4196",
         "PLZ NAME ME",
         "NOTAROBOT",
         "NEVERGOINGTO",
         "LETYOUDOWN",
         "GIVEYOUUP",
         "RUNAROUND",
         "ANDDESERTYOU",
         "MAKEYOUCRY",
         "SAYGOODBYE",
         "TELLALIE",
         "ANDHURTYOU",
         "MARVWASHERE",
         "Shi-Te-Addon",
         "TotalyClean",
         "NotInfected",
         "Not A Camera",
         "ChristItsHot",
         "Gen-er-ic",
         "WillUBMyFRND",
         "MKEPsGRTAGIN",
         "SomethinRude",
         "WinaaarIzU!",
         "NotRandom",
         "StimulusPkg",
         "PurpleThang",
         "CatchTheWave",
         "SexyTime",
         "UberEggPlant",
         "NoNameEggie",
         "NotAClue");

sub ascii_to_hex_str
{
  ($str,$size) = (@_);

  $outstr = "";
  for($n = 0 ; $n < $size ; $n++)
  {
    $outstr = $outstr . "00";
  }

  $hex_string = "";
  @chars = split("", $str);
  foreach $chr(@chars)
  {
    $hex_string = $hex_string . unpack "H*", $chr;
  }
  
  $outstr = substr($hex_string . $outstr, 0, $size * 2);

  return $outstr;
}

# generate serial number
for($n = 0 ; $n < 4 ; $n++)
{
  $SERIALNUM = $SERIALNUM . substr(sprintf("0%x",rand(255)),-2);
}

# select random name
$NAME = $names[rand @names];

$OPTIONS[1] = 0;
$OPTIONS[2] = 0;
# seed infections Chlamydia at 5%
if(rand(100) < 5)
{
  $OPTIONS[1] = $OPTIONS[1] | 64;
}

# seed infections Herpes at 10%
if(rand(100) < 10)
{
  $OPTIONS[1] = $OPTIONS[1] | 128;
  $OPTIONS[2] = $OPTIONS[2] | 128;
}


# print ascii_to_hex_str($TYPE, 9) . "\n";
# print ascii_to_hex_str($VERSION, 9) . "\n";
# print ascii_to_hex_str("", 9) . "\n";
# print "$SERIALNUM\n";
# print ascii_to_hex_str($SERIALNUM, 9) . "\n";

# print "$NAME\n";
# print ascii_to_hex_str($NAME, 13) . "\n";


$eeprom = ascii_to_hex_str($TYPE, 9)
        . ascii_to_hex_str($VERSION, 9);

foreach $val(@OPTIONS)
{
  $eeprom .= substr(uc sprintf("0%x",$val),-2);
}

$eeprom .= ascii_to_hex_str("", 9)
        . ascii_to_hex_str($SERIALNUM, 9)
        . ascii_to_hex_str($NAME, 13);

while(length $eeprom < $eeprom_size * 2)
{
  $eeprom .= "FF";
}

$prefix = "200";
$addr = 0;
$suffix = "000";

@arr = ( $eeprom =~ m/.{64}/g );

foreach $chr(@arr)
{
  $eepromStr = $prefix
    . substr(uc sprintf("0%x",$addr),-2)
    . $suffix
    . $chr;
  @chars = split '', $eepromStr;
  $checksum = 0;
  while (scalar(@chars))
  {
    $a = shift(@chars);
    $b = shift(@chars);
    $checksum += hex("$a$b");
  }
  printf ":$eepromStr%02X\n", 256-($checksum%256);
  $addr = $addr + 2;
}

print ":00000001FF\n";

open(FILE, ">>DB.txt") or die "oh nos!";
print FILE "$SERIALNUM,$OPTIONS[1],$OPTIONS[2],$NAME\n";
close(FILE);
