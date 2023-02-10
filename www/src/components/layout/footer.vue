<template>
  <div>
    <v-card absolute app color="primary" dark flat>
      <v-container>
        <v-row align="start">
          <v-col cols="12" md="4">
            <h4 class="text-h5 font-weight-bold">
              이경빈 (Bin Lee)
            </h4>
            <p class="text-body-1">
              4년제 대학 재학 중/ <br/>
              낭만 없이 코딩 못하는 사람/ <br/>
              클래식 기타리스트/ <br/>
              홈 바리스타 <br/>
            </p>
          </v-col>

          <v-col cols="12" md="4">
            <v-list color="transparent" flat>
              <v-subheader class="text-h6">외부 링크</v-subheader>
              <v-list-item-group color="primary">
                <v-list-item
                    v-for="(item, i) in binlee"
                    :key="i"
                    :href="item.href"
                    dense
                    target="_black"
                >
                  <v-list-item-content>
                    <v-list-item-title
                        class="text-body-1"
                        v-text="item.text"
                    ></v-list-item-title>
                  </v-list-item-content>
                </v-list-item>
              </v-list-item-group>
            </v-list>
          </v-col>

          <v-col cols="12" md="4">
            <div>
              <h5 class="text-h6 font-weight-bold">
                연락처
              </h5>
              <p class="text-body-1">
                egb3543@gmail.com
              </p>
            </div>
          </v-col>
        </v-row>
      </v-container>
    </v-card>
  </div>
</template>

<script>
export default {
  data: () => ({
    binlee: [
      {
        text: "Github",
        href: "https://github.com/binlee0903",
      },
      {
        text: "Facebook",
        href: "https://www.facebook.com/profile.php?id=100023724693989",
      },
    ]
  }),
};
</script>
