# Searcher

# Как запустить
- В случае, если обратный индекс, обратный индекс и словарь уже создан (./data/ids_urls.txt; ./data/index.txt, ./data/inverse index.txt)
```
  make
  ./Searcher
```
- В случае, если есть только папка data с lenta.ru_0i.txt:
```
  make --prep
  ./Searcher
```
# Формат запроса
- Слова вводятся через пробел
- Слова разделяются & или |
- Скобки отделены пробелом

Пример:
- docker & ubuntu & ( desk | fruit )


# Анализ Сжатия
При вызове ./Sercher --prep
- Index size is 5942568
- Compressed size is 2119098

Как видно, сжатие уменьшило размер словаря примерно в 3 раза. При этом лучше сжимались вектора с большим кол-вом слов -> при лемматизации
и поиску по большому набору данных сжатие становится ещё критичнее. При сжатии использовался varbyte.

# Анализ потоковой обработки
- naive search: 
```
его & её & ( морская | вода )
Time total = 0.000429
```
- stream search:
```
его & её & ( морская | вода )
Time total = 0.000388
```

Как видно, на 4 словах скорость работы 2 алгоритмов почти не отличаются, при это потоковая обработка использует O(n) доп памяти, где n "самого частого слова"
Также видно, что алгортим декордировки достаточно быстрый, чтобы использовать его мно-во раз при обработке запроса (При каждом доступе по индексы вектор
декодируется)

# Выводы
  - Потоковая обработка работает не медленее, чем наивная, при этом тратя меньше памяти
  - Частое декодирование массива при сжатии varbyte достаточно быстро, чтобы сжимать индекс и декодировать при каждом доступе
  - Сжатие индекса varbyte без лемматизации сжимает примерно в 3 раза
  - Без перехода в индексе к разности между docid сжатие составляло примерно 0.5 от начального размера
