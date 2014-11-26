program quicksort()

type arr is array of 1000 integer;

function qsort(a,h,t)
	var a is arr;
	var h is integer;
	var t is integer;
is
	var key is integer;
	var p1 is integer;
	var p2 is integer;
begin
	if h >= t then
		return;
	else
		key := a[h];
		p1 := h;
		p2 := t;
		while p1 < p2 do
			while p1 < p2 and a[p2] >= key do
				p2 := p2 - 1;
			end while
			while p1 < p2 and a[p1] < key do
				p1 := p1 + 1;
			end while
			if p1 < p2 then
				a[p2] := a[p1];
			end if
		end while
		a[p1] := key;
		qsort(a, h, p1-1);
		qsort(a, p2+1, t);
	end if
end function qsort;


is		
	var a is arr;
	var n is integer;
	var i is integer;
begin
	input n;
	i := 0;
	while i < n do
		input a[i];
		i := i + 1;
	end while
	qsort(a, 0, n-1);
	i := 0;
	while i < n do
		print a[i];
		i := i + 1;
	end while
end
