function ConnectWebSocket()
{
    const http = new XMLHttpRequest();
    const url = "www.binlee-blog.com/connect";
    http.setRequestHeader('Connection', 'Upgrade');
    http.setRequestHeader('Sec-WebSocket-Key', 'Upgrade');
    http.setRequestHeader('Sec-WebSocket-Version', '13');

    http.open('GET', url);

}

async function GetArticleFromServer(articleNumber)
{
    if (articleNumber == null)
    {
        return;
    }

    return await fetch('/getArticle', 
    {
        method: 'GET',
        headers: { 'Article-Number' : articleNumber }
    })
    .then((res) => 
    {
        return res.json();
    })
    .then((data) => 
    {
        return data;
    });
}

async function GetArticlesFromServer(pageIndex)
{
    if (pageIndex == null)
    {
        return;
    }

    return await fetch('/getArticles', 
    {
        method: 'GET',
        headers: { 'Page-Index' : pageIndex }
    })
    .then((res) => 
    {
        return res.json();
    })
    .then((data) =>
    {
        return data;
    });
}