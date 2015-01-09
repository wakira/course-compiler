program quickSort()
    type Array is array of 100 integer;
    function qsort (arr , low , high)
        var arr is Array;
        var low is integer;
        var high is integer;
        return Array;
    is
        var i is integer;
        var j is integer;
        var k is integer;
        var tmp is integer;
        var pivot is integer;
        var tmpArr is Array;
    begin
        i := low ;
        j := high ;
        pivot := arr [( low + high )/2];
        while i <= j do
            while arr [ i ] < pivot do
                i := i + 1 ;
            end while
            while arr [ j ] > pivot do
                j := j - 1;
            end while
            if i <=j then
                tmp := arr [ i ];
                arr [ i ] := arr [ j ];
                arr [ j ] := tmp;
                i := i + 1;
                j := j - 1;
            end if
        end while
        if low < j then
            tmpArr := qsort(arr, low, j);
            k := low;
            while k <= j do
               arr[k] := tmpArr[k];
               k := k + 1;
            end while
        end if
        if i < high then
            tmpArr := qsort(arr, i, high);
            k := i;
            while k <= high do
                arr[k] := tmpArr[k];
                k := k + 1;
            end while
        end if
        return arr;
    end function qsort;
is
    var arr is Array;
    var n is integer;
    var i is integer;
begin
    input n;
    i := 0;
    while i < n do
        input arr[i];
        i := i + 1;
    end while
    arr := qsort( arr , 0 , n - 1 );
    i := 0;
    while i < n do
        print arr[i];
        i := i + 1;
    end while

end
