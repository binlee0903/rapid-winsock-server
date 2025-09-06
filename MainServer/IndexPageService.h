#pragma once

class IndexPageService final : public Service
{
private:
	std::string INDEX_PAGE_SERVICE_NAME = "";

public:
	static IndexPageService* GetIndexPageServiceInstance(File* file);
	virtual ~IndexPageService();

	virtual uint64_t GetServiceName() const override;

	virtual bool Run(HttpObject* httpObject, int8_t** serviceOutput, int64_t* serviceOutputSize) const override;
	virtual bool Run(HttpObject* httpObject, std::string* serviceOutput) const override;
private:
	IndexPageService(int8_t* htmlPage, intmax_t fileSize);

private:
	static IndexPageService* mIndexPageService;
	int8_t* mHtmlPage;
	intmax_t mFileSize;
};