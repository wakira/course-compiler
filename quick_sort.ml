program quickSort()
    type Array is array of 100 integer;
    function qsort (arr , low , high)
        var arr is Array;
        var low is integer;
        var high is integer;
    is
        var i is integer;
        var j is integer;
        var tmp is integer;
        var pivot is integer;
    begin
        if high - low == 0 then
            print arr[low];
        end
        i := low ;
        j := high ;
        pivot := arr [ ( low + high )/2];
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
            qsort(arr, low, j);
        end if
        if i < high then
            qsort(arr, i, high);
        end if
end function qsort;
is
    var arr is Array;
    var n is integer;
    var i is integer;
begin
    n := 5;
    i := 0;
    while i < n do
        arr [ i ] := i;
        i := i + 1 ;
    end while
    qsort( arr , 0 , n - 1 );
end



