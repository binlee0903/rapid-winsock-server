function ConnectWebSocket()
{
    const http = new XMLHttpRequest();
    const url = "www.binlee-blog.com/connect";
    http.setRequestHeader('HOST', 'www.binlee-blog.com');
    http.setRequestHeader('Connection', 'Upgrade');
    http.setRequestHeader('Sec-WebSocket-Key', 'Upgrade');
    http.setRequestHeader('Sec-WebSocket-Version', '13');

    http.open('GET', url);

}

function GetArticleFromServer(articleNumber)
{
    const http = new XMLHttpRequest();
    const url = "www.binlee-blog.com/getArticle";
    http.setRequestHeader('HOST', 'www.binlee-blog.com');
    http.setRequestHeader('Article-Number', articleNumber);

    http.open('GET', url, false);
    http.send();

    http.onreadystatechange(() =>
    {
        if (httpRequest.readyState === XMLHttpRequest.DONE) {
            alert(httpRequest.responseText);
            let response = JSON.parse(httpRequest.responseText);
        } else {
            // 아직 준비되지 않음
        }
    });
}

function GetArticlesFromServer(pageIndex)
{
    const http = new XMLHttpRequest();
    const url = "www.binlee-blog.com/getArticles";
    http.setRequestHeader('HOST', 'www.binlee-blog.com');
    http.setRequestHeader('Page-Index', pageIndex);

    http.open('GET', url, false);
    http.send();

    http.onreadystatechange(() =>
    {
        if (httpRequest.readyState === XMLHttpRequest.DONE) {
            alert(httpRequest.responseText);
            let response = JSON.parse(httpRequest.responseText);
            PutArticlesToList(response);
        } else {
            // 아직 준비되지 않음
        }
    });
}