gcloud builds submit --tag gcr.io/driven-era-310811/moodcap-backend:0.0

gcloud run deploy --port=5000 --image gcr.io/driven-era-310811/moodcap-backend:0.0

gcloud functions deploy image-upload --entry-point MoodCap.PostImage --runtime dotnet3 --trigger-http --allow-unauthenticated


gcloud functions deploy image-list --entry-point MoodCap.ListImages --runtime dotnet3 --trigger-http --allow-unauthenticated

gcloud functions deploy data-upload --entry-point MoodCap.PostData --runtime dotnet3 --trigger-http --allow-unauthenticated
