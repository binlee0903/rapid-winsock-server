#pragma once

class IndexPageService final : public Service
{
private:
	std::string INDEX_PAGE_SERVICE_NAME = "";

public:
	static IndexPageService* GetIndexPageServiceInstance(std::vector<int8_t>* htmlPage);
	~IndexPageService() = default;

	virtual uint64_t GetServiceName() const override;
	virtual bool Run(HttpObject* httpObject, std::vector<int8_t>& serviceOutput) const override;
private:
	IndexPageService(std::vector<int8_t>* htmlPage);

private:
	static IndexPageService* mIndexPageService;
	std::vector<int8_t>* mHtmlPage;
};