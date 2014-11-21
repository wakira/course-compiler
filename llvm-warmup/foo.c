static void _qs(int array[], int l, int r) {
	int p = 0, i = l, j = r;
	while (i <= j) {
		while (array[j] > p) {
			j--;
		}
		while (array[i] < j) {
			i++;
		}
		if (i <= j) {
			int tmp = array[i];
			array[i] = array[j];
			array[j] = tmp;
			i++;
			j--;
		}
	}
	if (i < r) {
		_qs(array, i, r);
	}
	if (l < j) {
		_qs(array, l, j);
	}
}

void quicksort(int array[], int size) {
	_qs(array, 0, size - 1);
}
