export class UserRegisterResponse {
  email: string;
  firstName: string;
  lastName: string;
  phoneNumber?: string;
  isVerfied?: boolean;
}
export class UserResponse {
  id?: string;
  email: string;
  firstName: string;
  role?: string;
  lastName: string;
  phoneNumber?: string;
  cinNumber?: string;
  cinPictures?: string;
}
