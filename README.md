# dns_server

## TODO 
- Кнфиг формата ? yaml ? 
- рекурсивный поход в 8.8.8.8
- допарсить resources 
- формирование ответа
- перекладывание в байты 
- try except на случай ошибок 

## Имеем
-  Парсинг Header и Question
-  tcp и udp сервер на 53 порту 

## Запуск
```
make && sudo ./dns_server 
```
dns-сервер будет слушать 53 порт на localhost 