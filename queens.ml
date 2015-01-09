program queens()
    type Array is array of 8 integer;
    function abs(x)
        var x is integer;
        return integer;
    is
    begin
        if x < 0 then
            return -x;
        end if
        return x;
    end function abs;
    function place(arr, n)
        var arr is Array;
        var n is integer;
        return integer;
    is
        var i is integer;
        var j is integer;
        var flag is integer;
        var count is integer;
    begin
        if n == 8 then
            return 1;
        end if
        i := 0;
        count := 0;
        while i < 8 do
            j := 0;
            flag := 1;
            while j < n do
                if arr[j] == i or abs(j-n) == abs(arr[j] - i) then
                    flag := 0;
                end if
                j := j + 1;
            end while
            if flag == 1 then
                arr[n] := i;
                count := count + place(arr, n+1);
            end if
            i := i + 1;
        end while
        return count;
    end function place;
is
    var arr is Array;
    var i is integer;
begin
    i := 0;
    while i < 8 do
        arr[i] := -1;
        i := i + 1;
    end while
    print place(arr, 0);
end
