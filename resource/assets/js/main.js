(function main()
{
    GetArticleListFromServer(0)
    .then((data) =>
    {
        PutArticlesToList(data);
    });
})();