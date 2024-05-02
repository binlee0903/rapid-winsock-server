# rapid-winsock-server

Welcome! This project represents my blog's front and backend server,
developed using cpp with winsock2.

Average memory usage: 5MB

## How to use

Please add the following services to the json/services.json file:

index: Index of services
name: https://your-dns-url/name
file-name: Name of the related HTML or JSON file
method: HTTP method
level: Security level. Use whatever level you deem appropriate.

```
{
  "IndexPageService": {
    "index": 0,
    "name": "",
    "file-name": "index.html",
    "method": "GET",
    "level": "USER"
  },
  "GetArticleService": {
    "index": 1,
    "name": "getArticle",
    "method": "GET",
    "level": "USER"
  },
  "GetArticleListService": {
    "index": 2,
    "name": "getArticleList",
    "method": "GET",
    "level": "USER"
  }
}
```

Additionally, make sure to register the service in the constructor of HttpRouter.

```

// TODO : Add service list
	std::string serviceName = "";
	std::string pageName = json["IndexPageService"]["file-name"].asCString();
	mServices.insert({ mHash.GetHashValue(&serviceName), IndexPageService::GetIndexPageServiceInstance(mHttpFileContainer->GetFile(&pageName)) });

	serviceName = json["GetArticleService"]["name"].asCString();
	mServices.insert({ mHash.GetHashValue(&serviceName), GetArticleService::GetArticleServiceInstance(mSQLiteConnector) });

	serviceName = json["GetArticleListService"]["name"].asCString();
	mServices.insert({ mHash.GetHashValue(&serviceName), GetArticleListService::GetArticleListServiceInstance(mSQLiteConnector) });

```

And If you want to close server properly, Press 'q' to exit.

