program a()
type arr is array of 3 integer;
function ac(ab)
var ab is arr;
is
begin
print ab[0];
end function ac;
is
var c is integer;
var a is arr;
begin
c := 3;
a[0] := 99;
ac(a);
c := a[0];

end
