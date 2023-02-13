function PutArticlesToList(articles)
{
    let container = document.querySelector('#container');

    if (container == null)
    {
        return;
    }

    for (let i in articles.length)
    {
        container.innerHTML += `<div id="article-${i.index}" class="row gy-5 gx-4 g-xl-5">
            <div class="col-lg-6"> 
                <article class="card post-card h-100 border-0 bg-transparent"> 
                    <div class="card-body"> 
                        <a class="d-block" href="blog-single.html" title="${i.title}"> 
                            <div class="post-image position-relative"> 
                                <img class="w-100 h-auto rounded" src="${i.titleImagePath}" alt="${i.title}" width="970" height="500"> 
                            </div> 
                        </a> 
                        <ul class="card-meta list-inline mb-3"> 
                            <li class="list-inline-item mt-2"> 
                                <i class="ti ti-calendar-event"></i> 
                                <span>${i.date}</span> 
                            </li> 
                        </ul> 
                        <a id="article-${i.index}" class="d-block" href="blog-single.html" title="${i.title}"> 
                            <h3 class="mb-3 post-title"> 
                                ${i.title} 
                            </h3> 
                        </a>
                    </div> 
                </article> 
            </div> 
            <div class="col-12 text-center"> 
                <a class="btn btn-primary mt-5" href="blog.html" aria-label="View all posts"><i class="ti ti-new-section me-2"></i>View all posts</a> 
            </div> 
        </div>`;

        let article = document.querySelectorAll(`#article-${i.index}`);
        article[0].addEventListener('click', () =>
        {
            location.href = `blog-single.html?index=${i.index}`;
        });
        article[1].addEventListener('click', () =>
        {
            location.href = `blog-single.html?index=${i.index}`;
        });
    }
}

function PutArticlesToBlog(article)
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
        ${article.content}
      </div>
    </div>
  </div>`;
}