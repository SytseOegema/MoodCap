FROM mcr.microsoft.com/dotnet/sdk:5.0 AS build-env
WORKDIR /app


COPY *.csproj ./
RUN dotnet restore


COPY . ./
RUN dotnet publish -c Release -o out


FROM mcr.microsoft.com/dotnet/sdk:5.0
WORKDIR /app
COPY --from=build-env /app/out .

EXPOSE 5000
EXPOSE 5001
EXPOSE 8081

ENV ASPNETCORE_URLS "http://*:5000"
ENV ASPNETCORE_ENVIRONMENT "development"
ENV DOTNET_ENVIRONMENT "development"


ENTRYPOINT ["dotnet", "moodCap.dll"]
