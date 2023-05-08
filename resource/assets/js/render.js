function PutArticlesToList(articles)
{
    let container = document.querySelector('#container');

    if (container == null)
    {
        return;
    }

    for (let i in articles)
    {
        if (articles[i] == null)
        {
          continue;
        }

        container.innerHTML += `<div class="row gy-5 gx-4 g-xl-5">
            <div class="col-lg-6"> 
                <article class="card post-card h-100 border-0 bg-transparent"> 
                    <div class="card-body"> 
                        <a class="d-block" href="blog-single.html?index=${articles[i].index}" title="${articles[i].title}"> 
                            <div class="post-image position-relative"> 
                                <img class="w-100 h-auto rounded" src="${articles[i].titleImagePath}" alt="${articles[i].title}" width="970" height="500"> 
                            </div> 
                        </a> 
                        <ul class="card-meta list-inline mb-3"> 
                            <li class="list-inline-item mt-2"> 
                                <i class="ti ti-calendar-event"></i> 
                                <span>${articles[i].date}</span> 
                            </li> 
                        </ul> 
                        <a class="d-block" href="blog-single.html?index=${articles[i].index}" title="${articles[i].title}"> 
                            <h3 class="mb-3 post-title"> 
                                ${articles[i].title} 
                            </h3> 
                        </a>
                    </div> 
                </article> 
            </div>
        </div>`;
    }
}

function PutArticleToBlog(article)
{
    let container = document.querySelector('#blog-container');

    if (container == null)
    {
        return;
    }

    container.innerHTML += `<div class="row justify-content-center">
    <div class="col-lg-10">
      <div class="mb-5">
        <h3 class="h1 mb-4 post-title">${article.title}</h3>

        <ul class="card-meta list-inline mb-2">
          <li class="list-inline-item mt-2">
            <i class="ti ti-calendar-event"></i>
            <span>${article.date}</span>
          </li>
        </ul>
      </div>
    </div>
    <div class="col-lg-12">
      <div class="mb-5 text-center">
        <img class="w-100 h-auto rounded" src="${article.titleImagePath}" alt="${article.title}" width="970" height="500">
      </div>
    </div>
    <div class="col-lg-8 post-content-block order-0 order-lg-2">
      <div class="content">
        ${article.article}
      </div>
    </div>
  </div>`;
}