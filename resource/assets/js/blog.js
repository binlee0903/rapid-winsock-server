(function main()
{
    GetArticlesFromServer(0)
    .then((data) =>
    {
        PutArticlesToList(data);
    });
})();